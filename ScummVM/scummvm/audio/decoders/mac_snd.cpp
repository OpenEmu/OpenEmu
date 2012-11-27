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
 * The code in this file is based on information found at
 * http://developer.apple.com/legacy/mac/library/documentation/mac/Sound/Sound-60.html#HEADING60-15
 *
 * We implement both type 1 and type 2 snd resources, but only those that are sampled
 */

#include "common/textconsole.h"
#include "common/stream.h"

#include "audio/decoders/mac_snd.h"
#include "audio/decoders/raw.h"

namespace Audio {

SeekableAudioStream *makeMacSndStream(Common::SeekableReadStream *stream,
	DisposeAfterUse::Flag disposeAfterUse) {

	uint16 sndType = stream->readUint16BE();

	if (sndType == 1) {
		// "normal" snd resources
		if (stream->readUint16BE() != 1) {
			warning("makeMacSndStream(): Unsupported data type count");
			return 0;
		}

		if (stream->readUint16BE() != 5) {
			// 5 == sampled
			warning("makeMacSndStream(): Unsupported data type");
			return 0;
		}

		stream->readUint32BE(); // initialization option
	} else if (sndType == 2) {
		// old HyperCard snd resources
		stream->readUint16BE(); // reference count (unused)
	} else {
		warning("makeMacSndStream(): Unknown format type %d", sndType);
		return 0;
	}

	// We really should never get this as long as we have sampled data only
	if (stream->readUint16BE() != 1) {
		warning("makeMacSndStream(): Unsupported command count");
		return 0;
	}

	uint16 command = stream->readUint16BE();

	// 0x8050 - soundCmd  (with dataOffsetFlag set): install a sampled sound as a voice
	// 0x8051 - bufferCmd (with dataOffsetFlag set): play a sample sound
	if (command != 0x8050 && command != 0x8051) {
		warning("makeMacSndStream(): Unsupported command %04x", command);
		return 0;
	}

	stream->readUint16BE(); // 0
	uint32 soundHeaderOffset = stream->readUint32BE();

	stream->seek(soundHeaderOffset);

	uint32 soundDataOffset = stream->readUint32BE();
	uint32 size = stream->readUint32BE();
	uint16 rate = stream->readUint32BE() >> 16; // Really fixed point, but we only support integer rates
	stream->readUint32BE(); // loop start
	stream->readUint32BE(); // loop end
	byte encoding = stream->readByte();
	stream->readByte(); // base frequency

	if (encoding != 0) {
		// 0 == PCM
		warning("makeMacSndStream(): Unsupported compression %d", encoding);
		return 0;
	}

	stream->skip(soundDataOffset);

	byte *data = (byte *)malloc(size);
	assert(data);
	stream->read(data, size);

	if (disposeAfterUse == DisposeAfterUse::YES)
		delete stream;

	// Since we allocated our own buffer for the data, we must specify DisposeAfterUse::YES.
	return makeRawStream(data, size, rate, Audio::FLAG_UNSIGNED);
}

} // End of namespace Audio
