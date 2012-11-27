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

#include "audio/decoders/xa.h"
#include "audio/audiostream.h"
#include "common/stream.h"

namespace Audio {

class XAStream : public Audio::RewindableAudioStream {
public:
	XAStream(Common::SeekableReadStream *stream, int rate, DisposeAfterUse::Flag disposeAfterUse);
	~XAStream();

	bool isStereo() const { return false; }
	bool endOfData() const { return _endOfData && _samplesRemaining == 0; }
	int getRate() const { return _rate; }
	int readBuffer(int16 *buffer, const int numSamples);

	bool rewind();
private:
	Common::SeekableReadStream *_stream;
	DisposeAfterUse::Flag _disposeAfterUse;

	void seekToPos(uint pos);

	byte _predictor;
	double _samples[28];
	byte _samplesRemaining;
	int _rate;
	double _s1, _s2;
	uint _loopPoint;
	bool _endOfData;
};

XAStream::XAStream(Common::SeekableReadStream *stream, int rate, DisposeAfterUse::Flag disposeAfterUse)
		: _stream(stream), _disposeAfterUse(disposeAfterUse) {
	_samplesRemaining = 0;
	_predictor = 0;
	_s1 = _s2 = 0.0;
	_rate = rate;
	_loopPoint = 0;
	_endOfData = false;
}


XAStream::~XAStream() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete _stream;
}

static const double s_xaDataTable[5][2] = {
	{  0.0, 0.0 },
	{  60.0 / 64.0,  0.0 },
	{  115.0 / 64.0, -52.0 / 64.0 },
	{  98.0 / 64.0, -55.0 / 64.0 },
	{  122.0 / 64.0, -60.0 / 64.0 }
};

int XAStream::readBuffer(int16 *buffer, const int numSamples) {
	int32 samplesDecoded = 0;

	for (int i = 28 - _samplesRemaining; i < 28 && samplesDecoded < numSamples; i++) {
		_samples[i] = _samples[i] + _s1 * s_xaDataTable[_predictor][0] + _s2 * s_xaDataTable[_predictor][1];
		_s2 = _s1;
		_s1 = _samples[i];
		int16 d = (int) (_samples[i] + 0.5);
		buffer[samplesDecoded] = d;
		samplesDecoded++;
		_samplesRemaining--;
	}

	if (endOfData())
		return samplesDecoded;

	while (samplesDecoded < numSamples) {
		byte i = 0;

		_predictor = _stream->readByte();
		byte shift = _predictor & 0xf;
		_predictor >>= 4;

		byte flags = _stream->readByte();
		if (flags == 3) {
			// Loop
			seekToPos(_loopPoint);
			continue;
		} else if (flags == 6) {
			// Set loop point
			_loopPoint = _stream->pos() - 2;
		} else if (flags == 7) {
			// End of stream
			_endOfData = true;
			return samplesDecoded;
		}

		for (i = 0; i < 28; i += 2) {
			byte d = _stream->readByte();
			int16 s = (d & 0xf) << 12;
			if (s & 0x8000)
				s |= 0xffff0000;
			_samples[i] = (double)(s >> shift);
			s = (d & 0xf0) << 8;
			if (s & 0x8000)
				s |= 0xffff0000;
			_samples[i + 1] = (double)(s >> shift);
		}

		for (i = 0; i < 28 && samplesDecoded < numSamples; i++) {
			_samples[i] = _samples[i] + _s1 * s_xaDataTable[_predictor][0] + _s2 * s_xaDataTable[_predictor][1];
			_s2 = _s1;
			_s1 = _samples[i];
			int16 d = (int) (_samples[i] + 0.5);
			buffer[samplesDecoded] = d;
			samplesDecoded++;
		}

		if (i != 28)
			_samplesRemaining = 28 - i;

		if (_stream->pos() >= _stream->size())
			_endOfData = true;
	}

	return samplesDecoded;
}

bool XAStream::rewind() {
	seekToPos(0);
	return true;
}

void XAStream::seekToPos(uint pos) {
	_stream->seek(pos);
	_samplesRemaining = 0;
	_predictor = 0;
	_s1 = _s2 = 0.0;
	_endOfData = false;
}

RewindableAudioStream *makeXAStream(Common::SeekableReadStream *stream, int rate, DisposeAfterUse::Flag disposeAfterUse) {
	return new XAStream(stream, rate, disposeAfterUse);
}

} // End of namespace Audio
