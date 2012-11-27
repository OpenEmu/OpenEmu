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
#include "common/textconsole.h"
#include "common/util.h"

#include "audio/decoders/adpcm.h"
#include "audio/decoders/adpcm_intern.h"


namespace Audio {

// Routines to convert 12 bit linear samples to the
// Dialogic or Oki ADPCM coding format aka VOX.
// See also <http://www.comptek.ru/telephony/tnotes/tt1-13.html>
//
// IMA ADPCM support is based on
//   <http://wiki.multimedia.cx/index.php?title=IMA_ADPCM>
//
// In addition, also MS IMA ADPCM is supported. See
//   <http://wiki.multimedia.cx/index.php?title=Microsoft_IMA_ADPCM>.

ADPCMStream::ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
	: _stream(stream, disposeAfterUse),
		_startpos(stream->pos()),
		_endpos(_startpos + size),
		_channels(channels),
		_blockAlign(blockAlign),
		_rate(rate) {

	reset();
}

void ADPCMStream::reset() {
	memset(&_status, 0, sizeof(_status));
	_blockPos[0] = _blockPos[1] = _blockAlign; // To make sure first header is read
}

bool ADPCMStream::rewind() {
	// TODO: Error checking.
	reset();
	_stream->seek(_startpos);
	return true;
}


#pragma mark -


int Oki_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples;
	byte data;

	for (samples = 0; samples < numSamples && !endOfData(); samples++) {
		if (_decodedSampleCount == 0) {
			data = _stream->readByte();
			_decodedSamples[0] = decodeOKI((data >> 4) & 0x0f);
			_decodedSamples[1] = decodeOKI((data >> 0) & 0x0f);
			_decodedSampleCount = 2;
		}

		// (1 - (count - 1)) ensures that _decodedSamples acts as a FIFO of depth 2
		buffer[samples] = _decodedSamples[1 - (_decodedSampleCount - 1)];
		_decodedSampleCount--;
	}

	return samples;
}

static const int16 okiStepSize[49] = {
	   16,   17,   19,   21,   23,   25,   28,   31,
	   34,   37,   41,   45,   50,   55,   60,   66,
	   73,   80,   88,   97,  107,  118,  130,  143,
	  157,  173,  190,  209,  230,  253,  279,  307,
	  337,  371,  408,  449,  494,  544,  598,  658,
	  724,  796,  876,  963, 1060, 1166, 1282, 1411,
	 1552
};

// Decode Linear to ADPCM
int16 Oki_ADPCMStream::decodeOKI(byte code) {
	int16 diff, E, samp;

	E = (2 * (code & 0x7) + 1) * okiStepSize[_status.ima_ch[0].stepIndex] / 8;
	diff = (code & 0x08) ? -E : E;
	samp = _status.ima_ch[0].last + diff;
	// Clip the values to +/- 2^11 (supposed to be 12 bits)
	samp = CLIP<int16>(samp, -2048, 2047);

	_status.ima_ch[0].last = samp;
	_status.ima_ch[0].stepIndex += _stepAdjustTable[code];
	_status.ima_ch[0].stepIndex = CLIP<int32>(_status.ima_ch[0].stepIndex, 0, ARRAYSIZE(okiStepSize) - 1);

	// * 16 effectively converts 12-bit input to 16-bit output
	return samp * 16;
}


#pragma mark -


int DVI_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples;
	byte data;

	for (samples = 0; samples < numSamples && !endOfData(); samples++) {
		if (_decodedSampleCount == 0) {
			data = _stream->readByte();
			_decodedSamples[0] = decodeIMA((data >> 4) & 0x0f, 0);
			_decodedSamples[1] = decodeIMA((data >> 0) & 0x0f, _channels == 2 ? 1 : 0);
			_decodedSampleCount = 2;
		}

		// (1 - (count - 1)) ensures that _decodedSamples acts as a FIFO of depth 2
		buffer[samples] = _decodedSamples[1 - (_decodedSampleCount - 1)];
		_decodedSampleCount--;
	}

	return samples;
}

#pragma mark -


int Apple_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	// Need to write at least one samples per channel
	assert((numSamples % _channels) == 0);

	// Current sample positions
	int samples[2] = { 0, 0};

	// Number of samples per channel
	int chanSamples = numSamples / _channels;

	for (int i = 0; i < _channels; i++) {
		_stream->seek(_streamPos[i]);

		while ((samples[i] < chanSamples) &&
		       // Last byte read and a new one needed
		       !((_stream->eos() || (_stream->pos() >= _endpos)) && (_chunkPos[i] == 0))) {

			if (_blockPos[i] == _blockAlign) {
				// 2 byte header per block
				uint16 temp = _stream->readUint16BE();

				// First 9 bits are the upper bits of the predictor
				_status.ima_ch[i].last      = (int16) (temp & 0xFF80);
				// Lower 7 bits are the step index
				_status.ima_ch[i].stepIndex =          temp & 0x007F;

				// Clip the step index
				_status.ima_ch[i].stepIndex = CLIP<int32>(_status.ima_ch[i].stepIndex, 0, 88);

				_blockPos[i] = 2;
			}

			if (_chunkPos[i] == 0) {
				// Decode data
				byte data = _stream->readByte();
				_buffer[i][0] = decodeIMA(data &  0x0F, i);
				_buffer[i][1] = decodeIMA(data >>    4, i);
			}

			// The original is interleaved block-wise, we want it sample-wise
			buffer[_channels * samples[i] + i] = _buffer[i][_chunkPos[i]];

			if (++_chunkPos[i] > 1) {
				// We're about to decode the next byte, so advance the block position
				_chunkPos[i] = 0;
				_blockPos[i]++;
			}

			samples[i]++;

			if (_channels == 2)
				if (_blockPos[i] == _blockAlign)
					// We're at the end of the block.
					// Since the channels are interleaved, skip the next block
					_stream->skip(MIN<uint32>(_blockAlign, _endpos - _stream->pos()));

			_streamPos[i] = _stream->pos();
		}
	}

	return samples[0] + samples[1];
}


#pragma mark -


int MSIma_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	// Need to write at least one sample per channel
	assert((numSamples % _channels) == 0);

	int samples = 0;

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos[0] == _blockAlign) {
			for (int i = 0; i < _channels; i++) {
				// read block header
				_status.ima_ch[i].last = _stream->readSint16LE();
				_status.ima_ch[i].stepIndex = _stream->readSint16LE();
			}

			_blockPos[0] = _channels * 4;
		}

		// Decode a set of samples
		for (int i = 0; i < _channels; i++) {
			// The stream encodes four bytes per channel at a time
			for (int j = 0; j < 4; j++) {
				byte data = _stream->readByte();
				_blockPos[0]++;
				_buffer[i][j * 2] = decodeIMA(data & 0x0f, i);
				_buffer[i][j * 2 + 1] = decodeIMA((data >> 4) & 0x0f, i);
				_samplesLeft[i] += 2;
			}
		}

		while (samples < numSamples && _samplesLeft[0] != 0) {
			for (int i = 0; i < _channels; i++) {
				buffer[samples + i] = _buffer[i][8 - _samplesLeft[i]];
				_samplesLeft[i]--;
			}

			samples += _channels;
		}
	}

	return samples;
}


#pragma mark -


static const int MSADPCMAdaptCoeff1[] = {
	256, 512, 0, 192, 240, 460, 392
};

static const int MSADPCMAdaptCoeff2[] = {
	0, -256, 0, 64, 0, -208, -232
};

static const int MSADPCMAdaptationTable[] = {
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230
};


int16 MS_ADPCMStream::decodeMS(ADPCMChannelStatus *c, byte code) {
	int32 predictor;

	predictor = (((c->sample1) * (c->coeff1)) + ((c->sample2) * (c->coeff2))) / 256;
	predictor += (signed)((code & 0x08) ? (code - 0x10) : (code)) * c->delta;

	predictor = CLIP<int32>(predictor, -32768, 32767);

	c->sample2 = c->sample1;
	c->sample1 = predictor;
	c->delta = (MSADPCMAdaptationTable[(int)code] * c->delta) >> 8;

	if (c->delta < 16)
		c->delta = 16;

	return (int16)predictor;
}

int MS_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples;
	byte data;
	int i = 0;

	samples = 0;

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos[0] == _blockAlign) {
			// read block header
			for (i = 0; i < _channels; i++) {
				_status.ch[i].predictor = CLIP(_stream->readByte(), (byte)0, (byte)6);
				_status.ch[i].coeff1 = MSADPCMAdaptCoeff1[_status.ch[i].predictor];
				_status.ch[i].coeff2 = MSADPCMAdaptCoeff2[_status.ch[i].predictor];
			}

			for (i = 0; i < _channels; i++)
				_status.ch[i].delta = _stream->readSint16LE();

			for (i = 0; i < _channels; i++)
				_status.ch[i].sample1 = _stream->readSint16LE();

			for (i = 0; i < _channels; i++)
				buffer[samples++] = _status.ch[i].sample2 = _stream->readSint16LE();

			for (i = 0; i < _channels; i++)
				buffer[samples++] = _status.ch[i].sample1;

			_blockPos[0] = _channels * 7;
		}

		for (; samples < numSamples && _blockPos[0] < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples += 2) {
			data = _stream->readByte();
			_blockPos[0]++;
			buffer[samples] = decodeMS(&_status.ch[0], (data >> 4) & 0x0f);
			buffer[samples + 1] = decodeMS(&_status.ch[_channels - 1], data & 0x0f);
		}
	}

	return samples;
}


#pragma mark -


#define DK3_READ_NIBBLE() \
do { \
	if (_topNibble) { \
		_nibble = _lastByte >> 4; \
		_topNibble = false; \
	} else { \
		if (_stream->pos() >= _endpos) \
			break; \
		if ((_stream->pos() % _blockAlign) == 0) \
			continue; \
		_lastByte = _stream->readByte(); \
		_nibble = _lastByte & 0xf; \
		_topNibble = true; \
	} \
} while (0)


int DK3_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;

	assert((numSamples % 4) == 0);

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if ((_stream->pos() % _blockAlign) == 0) {
			_stream->readUint16LE(); // Unknown
			uint16 rate = _stream->readUint16LE(); // Copy of rate
			_stream->skip(6); // Unknown
			// Get predictor for both sum/diff channels
			_status.ima_ch[0].last = _stream->readSint16LE();
			_status.ima_ch[1].last = _stream->readSint16LE();
			// Get index for both sum/diff channels
			_status.ima_ch[0].stepIndex = _stream->readByte();
			_status.ima_ch[1].stepIndex = _stream->readByte();

			if (_stream->eos())
				break;

			// Sanity check
			assert(rate == getRate());
		}

		DK3_READ_NIBBLE();
		decodeIMA(_nibble, 0);

		DK3_READ_NIBBLE();
		decodeIMA(_nibble, 1);

		buffer[samples++] = _status.ima_ch[0].last + _status.ima_ch[1].last;
		buffer[samples++] = _status.ima_ch[0].last - _status.ima_ch[1].last;

		DK3_READ_NIBBLE();
		decodeIMA(_nibble, 0);

		buffer[samples++] = _status.ima_ch[0].last + _status.ima_ch[1].last;
		buffer[samples++] = _status.ima_ch[0].last - _status.ima_ch[1].last;
	}

	return samples;
}


#pragma mark -


// This table is used to adjust the step for use on the next sample.
// We could half the table, but since the lookup index used is always
// a 4-bit nibble, it's more efficient to just keep it as it is.
const int16 ADPCMStream::_stepAdjustTable[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
};

const int16 Ima_ADPCMStream::_imaTable[89] = {
		7,    8,    9,   10,   11,   12,   13,   14,
	   16,   17,   19,   21,   23,   25,   28,   31,
	   34,   37,   41,   45,   50,   55,   60,   66,
	   73,   80,   88,   97,  107,  118,  130,  143,
	  157,  173,  190,  209,  230,  253,  279,  307,
	  337,  371,  408,  449,  494,  544,  598,  658,
	  724,  796,  876,  963, 1060, 1166, 1282, 1411,
	 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
	 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
	 7132, 7845, 8630, 9493,10442,11487,12635,13899,
	15289,16818,18500,20350,22385,24623,27086,29794,
	32767
};

int16 Ima_ADPCMStream::decodeIMA(byte code, int channel) {
	int32 E = (2 * (code & 0x7) + 1) * _imaTable[_status.ima_ch[channel].stepIndex] / 8;
	int32 diff = (code & 0x08) ? -E : E;
	int32 samp = CLIP<int32>(_status.ima_ch[channel].last + diff, -32768, 32767);

	_status.ima_ch[channel].last = samp;
	_status.ima_ch[channel].stepIndex += _stepAdjustTable[code];
	_status.ima_ch[channel].stepIndex = CLIP<int32>(_status.ima_ch[channel].stepIndex, 0, ARRAYSIZE(_imaTable) - 1);

	return samp;
}

RewindableAudioStream *makeADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, typesADPCM type, int rate, int channels, uint32 blockAlign) {
	// If size is 0, report the entire size of the stream
	if (!size)
		size = stream->size();

	switch (type) {
	case kADPCMOki:
		return new Oki_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	case kADPCMMSIma:
		return new MSIma_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	case kADPCMMS:
		return new MS_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	case kADPCMDVI:
		return new DVI_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	case kADPCMApple:
		return new Apple_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	case kADPCMDK3:
		return new DK3_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	default:
		error("Unsupported ADPCM encoding");
		break;
	}
}

} // End of namespace Audio
