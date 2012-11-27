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

#include "groovie/lzss.h"

namespace Groovie {

#define OUT_BUFF_SIZE 131072
#define COMP_THRESH 3 // Compression not attempted if string to be compressed is less than 3 long

LzssReadStream::LzssReadStream(Common::ReadStream *indata, uint8 lengthmask, uint8 lengthbits) {
	/*
	TODO: Nasty hack. Make a buffer bigger than I'll ever need... probably.
	What should *really* happen is I should define a whole new type of stream
	that gets lzss decompressed on the fly
	*/
	_outLzssBufData = (uint8 *)malloc(OUT_BUFF_SIZE);
	_size = decodeLZSS(indata, lengthmask, lengthbits);
	_pos = 0;
}

LzssReadStream::~LzssReadStream() {
	free(_outLzssBufData);
}

uint32 LzssReadStream::decodeLZSS(Common::ReadStream *in, uint8 lengthmask, uint8 lengthbits) {
	uint32 N = 1 << (16 - lengthbits); /* History buffer size */
	byte *histbuff = new byte[N]; /* History buffer */
	memset(histbuff, 0, N);
	uint32 outstreampos = 0;
	uint32 bufpos = 0;

	while (!in->eos()) {
		byte flagbyte = in->readByte();
		for (uint32 i = 1; i <= 8; i++) {
			if (!in->eos()) {
				if ((flagbyte & 1) == 0) {
					uint32 offsetlen = in->readUint16LE();
					if (offsetlen == 0) {
						break;
					}
					uint32 length = (offsetlen & lengthmask) + COMP_THRESH;
					uint32 offset = (bufpos - (offsetlen >> lengthbits)) & (N - 1);
					for (uint32 j = 0; j < length; j++) {
						byte tempa = histbuff[(offset + j) & (N - 1)];
						_outLzssBufData[outstreampos++] = tempa;
						histbuff[bufpos] = tempa;
						bufpos = (bufpos + 1) & (N - 1);
					}
				} else {
					byte tempa = in->readByte();
					if (in->eos()) {
						break;
					}
					_outLzssBufData[outstreampos++] = tempa;
					histbuff[bufpos] = tempa;
					bufpos = (bufpos + 1) & (N - 1);
				}
				flagbyte = flagbyte >> 1;
			}
		}
	}
	delete[] histbuff;
	return outstreampos;
}

bool LzssReadStream::eos() const {
	return _pos >= _size;
}

uint32 LzssReadStream::read(void *buf, uint32 size) {
	if (size > _size - _pos)
		size = _size - _pos;

	memcpy(buf, &_outLzssBufData[_pos], size);
	_pos += size;

	return size;
}

} // End of Groovie namespace
