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
 * http://www.borg.com/~jglatt/tech/aiff.htm
 *
 * We currently only implement uncompressed AIFF. If we ever need AIFF-C, SoX
 * (http://sox.sourceforge.net) may be a good place to start from.
 */

#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "audio/decoders/aiff.h"
#include "audio/decoders/raw.h"

namespace Audio {

uint32 readExtended(Common::SeekableReadStream &stream) {
	// The sample rate is stored as an "80 bit IEEE Standard 754 floating
	// point number (Standard Apple Numeric Environment [SANE] data type
	// Extended).

	byte buf[10];
	uint32 mantissa;
	uint32 last = 0;
	byte exp;

	stream.read(buf, 10);
	mantissa = READ_BE_UINT32(buf + 2);
	exp = 30 - buf[1];

	while (exp--) {
		last = mantissa;
		mantissa >>= 1;
	}

	if (last & 0x00000001)
		mantissa++;

	return mantissa;
}

bool loadAIFFFromStream(Common::SeekableReadStream &stream, int &size, int &rate, byte &flags) {
	byte buf[4];

	stream.read(buf, 4);
	if (memcmp(buf, "FORM", 4) != 0) {
		warning("loadAIFFFromStream: No 'FORM' header");
		return false;
	}

	stream.readUint32BE();

	// This could be AIFC, but we don't handle that case.

	stream.read(buf, 4);
	if (memcmp(buf, "AIFF", 4) != 0) {
		warning("loadAIFFFromStream: No 'AIFF' header");
		return false;
	}

	// From here on, we only care about the COMM and SSND chunks, which are
	// the only required chunks.

	bool foundCOMM = false;
	bool foundSSND = false;

	uint16 numChannels = 0, bitsPerSample = 0;
	uint32 numSampleFrames = 0, offset = 0, blockSize = 0, soundOffset = 0;

	while (!(foundCOMM && foundSSND) && !stream.err() && !stream.eos()) {
		uint32 length, pos;

		stream.read(buf, 4);
		length = stream.readUint32BE();
		pos = stream.pos();

		if (memcmp(buf, "COMM", 4) == 0) {
			foundCOMM = true;
			numChannels = stream.readUint16BE();
			numSampleFrames = stream.readUint32BE();
			bitsPerSample = stream.readUint16BE();
			rate = readExtended(stream);
			size = numSampleFrames * numChannels * (bitsPerSample / 8);
		} else if (memcmp(buf, "SSND", 4) == 0) {
			foundSSND = true;
			offset = stream.readUint32BE();
			blockSize = stream.readUint32BE();
			soundOffset = stream.pos();
		}

		stream.seek(pos + length);
	}

	if (!foundCOMM) {
		warning("loadAIFFFromStream: Cound not find 'COMM' chunk");
		return false;
	}

	if (!foundSSND) {
		warning("loadAIFFFromStream: Cound not find 'SSND' chunk");
		return false;
	}

	// We only implement a subset of the AIFF standard.

	if (numChannels < 1 || numChannels > 2) {
		warning("loadAIFFFromStream: Only 1 or 2 channels are supported, not %d", numChannels);
		return false;
	}

	if (bitsPerSample != 8 && bitsPerSample != 16) {
		warning("loadAIFFFromStream: Only 8 or 16 bits per sample are supported, not %d", bitsPerSample);
		return false;
	}

	if (offset != 0 || blockSize != 0) {
		warning("loadAIFFFromStream: Block-aligned data is not supported");
		return false;
	}

	// Samples are always signed, and big endian.

	flags = 0;
	if (bitsPerSample == 16)
		flags |= Audio::FLAG_16BITS;
	if (numChannels == 2)
		flags |= Audio::FLAG_STEREO;

	stream.seek(soundOffset);

	// Stream now points at the sample data

	return true;
}

SeekableAudioStream *makeAIFFStream(Common::SeekableReadStream *stream,
	DisposeAfterUse::Flag disposeAfterUse) {
	int size, rate;
	byte *data, flags;

	if (!loadAIFFFromStream(*stream, size, rate, flags)) {
		if (disposeAfterUse == DisposeAfterUse::YES)
			delete stream;
		return 0;
	}

	data = (byte *)malloc(size);
	assert(data);
	stream->read(data, size);

	if (disposeAfterUse == DisposeAfterUse::YES)
		delete stream;

	// Since we allocated our own buffer for the data, we must specify DisposeAfterUse::YES.
	return makeRawStream(data, size, rate, flags);
}

} // End of namespace Audio
