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

#ifndef TINSEL_ADPCM_H
#define TINSEL_ADPCM_H

#include "audio/decoders/adpcm_intern.h"

namespace Tinsel {

class Tinsel_ADPCMStream : public Audio::ADPCMStream {
protected:
	struct {
		// Tinsel
		double predictor;
		double K0, K1;
		double d0, d1;
	} _status;

	void reset() {
		ADPCMStream::reset();
		memset(&_status, 0, sizeof(_status));
	}

	int16 decodeTinsel(int16, double);
	void readBufferTinselHeader();

public:
	Tinsel_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {

		if (blockAlign == 0)
			error("Tinsel_ADPCMStream(): blockAlign isn't specified");

		if (channels != 1)
			error("Tinsel_ADPCMStream(): Tinsel ADPCM only supports mono");

		memset(&_status, 0, sizeof(_status));
	}

};

class Tinsel4_ADPCMStream : public Tinsel_ADPCMStream {
public:
	Tinsel4_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Tinsel_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {}

	virtual int readBuffer(int16 *buffer, const int numSamples);
};

class Tinsel6_ADPCMStream : public Tinsel_ADPCMStream {
protected:
	uint8 _chunkPos;
	uint16 _chunkData;

	void reset() {
		ADPCMStream::reset();
		_chunkPos = 0;
		_chunkData = 0;
	}

public:
	Tinsel6_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Tinsel_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {
		_chunkPos = 0;
		_chunkData = 0;
	}

	virtual int readBuffer(int16 *buffer, const int numSamples);
};

class Tinsel8_ADPCMStream : public Tinsel_ADPCMStream {
public:
	Tinsel8_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Tinsel_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {}

	virtual int readBuffer(int16 *buffer, const int numSamples);
};


} // End of namespace Tinsel

#endif
