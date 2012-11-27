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

#include "audio/decoders/iff_sound.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/iff_container.h"
#include "common/func.h"

namespace Audio {

struct Voice8Header {
	uint32	oneShotHiSamples;
	uint32	repeatHiSamples;
	uint32	samplesPerHiCycle;
	uint16	samplesPerSec;
	byte	octaves;
	byte	compression;
	uint32	volume;

	Voice8Header() {
		memset(this, 0, sizeof(Voice8Header));
	}

	void load(Common::ReadStream &stream);
};

void Voice8Header::load(Common::ReadStream &stream) {
	oneShotHiSamples = stream.readUint32BE();
	repeatHiSamples = stream.readUint32BE();
	samplesPerHiCycle = stream.readUint32BE();
	samplesPerSec = stream.readUint16BE();
	octaves = stream.readByte();
	compression = stream.readByte();
	volume = stream.readUint32BE();
}



struct A8SVXLoader {
	Voice8Header _header;
	int8 *_data;
	uint32 _dataSize;

	void load(Common::ReadStream &input) {
		Common::IFFParser parser(&input);
		Common::Functor1Mem< Common::IFFChunk&, bool, A8SVXLoader > c(this, &A8SVXLoader::callback);
		parser.parse(c);
	}

	bool callback(Common::IFFChunk &chunk) {
		switch (chunk._type) {
		case ID_VHDR:
			_header.load(*chunk._stream);
			break;

		case ID_BODY:
			_dataSize = chunk._size;
			_data = (int8 *)malloc(_dataSize);
			assert(_data);
			loadData(chunk._stream);
			return true;
		}

		return false;
	}

	void loadData(Common::ReadStream *stream) {
		switch (_header.compression) {
		case 0:
			stream->read(_data, _dataSize);
			break;

		case 1:
			// implement other formats here
			error("compressed IFF audio is not supported");
			break;
		}

	}
};


AudioStream *make8SVXStream(Common::ReadStream &input, bool loop) {
	A8SVXLoader loader;
	loader.load(input);

	SeekableAudioStream *stream = Audio::makeRawStream((byte *)loader._data, loader._dataSize, loader._header.samplesPerSec, 0);

	uint32 loopStart = 0, loopEnd = 0;
	if (loop) {
		// the standard way to loop 8SVX audio implies use of the oneShotHiSamples and
		// repeatHiSamples fields
		loopStart = 0;
		loopEnd = loader._header.oneShotHiSamples + loader._header.repeatHiSamples;

		if (loopStart != loopEnd) {
			return new SubLoopingAudioStream(stream, 0,
					Timestamp(0, loopStart, loader._header.samplesPerSec),
					Timestamp(0, loopEnd, loader._header.samplesPerSec));
		}
	}

	return stream;
}

}
