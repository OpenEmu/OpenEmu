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

#include "common/stream.h"
#include "common/util.h"

#include "tinsel/adpcm.h"

namespace Tinsel {

static const double TinselFilterTable[4][2] = {
	{0, 0 },
	{0.9375, 0},
	{1.796875, -0.8125},
	{1.53125, -0.859375}
};

void Tinsel_ADPCMStream::readBufferTinselHeader() {
	uint8 start = _stream->readByte();
	uint8 filterVal = (start & 0xC0) >> 6;

	if ((start & 0x20) != 0) {
		//Lower 6 bit are negative

		// Negate
		start = ~(start | 0xC0) + 1;

		_status.predictor = (unsigned long long int)1 << start;
	} else {
		// Lower 6 bit are positive

		// Truncate
		start &= 0x1F;

		_status.predictor = ((double) 1.0) / ((unsigned long long int)1 << start);
	}

	_status.K0 = TinselFilterTable[filterVal][0];
	_status.K1 = TinselFilterTable[filterVal][1];
}

int16 Tinsel_ADPCMStream::decodeTinsel(int16 code, double eVal) {
	double sample;

	sample = (double)code;
	sample *= eVal * _status.predictor;
	sample += (_status.d0 * _status.K0) + (_status.d1 * _status.K1);

	_status.d1 = _status.d0;
	_status.d0 = sample;

	return (int16) CLIP<double>(sample, -32768.0, 32767.0);
}

int Tinsel4_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples;
	uint16 data;
	const double eVal = 1.142822265;

	samples = 0;

	assert(numSamples % 2 == 0);

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos[0] == _blockAlign) {
			readBufferTinselHeader();
			_blockPos[0] = 0;
		}

		for (; samples < numSamples && _blockPos[0] < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples += 2, _blockPos[0]++) {
			// Read 1 byte = 8 bits = two 4 bit blocks
			data = _stream->readByte();
			buffer[samples] = decodeTinsel((data << 8) & 0xF000, eVal);
			buffer[samples+1] = decodeTinsel((data << 12) & 0xF000, eVal);
		}
	}

	return samples;
}

int Tinsel6_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples;
	const double eVal = 1.032226562;

	samples = 0;

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos[0] == _blockAlign) {
			readBufferTinselHeader();
			_blockPos[0] = 0;
			_chunkPos = 0;
		}

		for (; samples < numSamples && _blockPos[0] < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples++, _chunkPos = (_chunkPos + 1) % 4) {

			switch (_chunkPos) {
			case 0:
				_chunkData = _stream->readByte();
				buffer[samples] = decodeTinsel((_chunkData << 8) & 0xFC00, eVal);
				break;
			case 1:
				_chunkData = (_chunkData << 8) | (_stream->readByte());
				buffer[samples] = decodeTinsel((_chunkData << 6) & 0xFC00, eVal);
				_blockPos[0]++;
				break;
			case 2:
				_chunkData = (_chunkData << 8) | (_stream->readByte());
				buffer[samples] = decodeTinsel((_chunkData << 4) & 0xFC00, eVal);
				_blockPos[0]++;
				break;
			case 3:
				_chunkData = (_chunkData << 8);
				buffer[samples] = decodeTinsel((_chunkData << 2) & 0xFC00, eVal);
				_blockPos[0]++;
				break;
			}

		}

	}

	return samples;
}

int Tinsel8_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples;
	byte data;
	const double eVal = 1.007843258;

	samples = 0;

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos[0] == _blockAlign) {
			readBufferTinselHeader();
			_blockPos[0] = 0;
		}

		for (; samples < numSamples && _blockPos[0] < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples++, _blockPos[0]++) {
			// Read 1 byte = 8 bits = one 8 bit block
			data = _stream->readByte();
			buffer[samples] = decodeTinsel(data << 8, eVal);
		}
	}

	return samples;
}


} // End of namespace Tinsel
